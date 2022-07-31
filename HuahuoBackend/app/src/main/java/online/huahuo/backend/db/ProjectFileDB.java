package online.huahuo.backend.db;

import lombok.Data;
import lombok.NoArgsConstructor;

import javax.persistence.*;

@Entity
@Data
@NoArgsConstructor
@Table(name = "PROJECTS", indexes = {
        @Index(columnList = "name"),
        @Index(columnList = "createdByUserId")
})
public class ProjectFileDB {
    @Id
    @Column(name = "id", nullable = false)
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    private String name;
    private String type;
    private String version;

    private Long createdByUserId;
    private String path;
    private String checksum;

    public ProjectFileDB(String name, String type, String version, Long createdByUserId,
                         String path, String checksum){
        this.name = name;
        this.type = type;
        this.version = version;
        this.path = path;
        this.checksum = checksum;
        this.createdByUserId = createdByUserId;
    }
}
